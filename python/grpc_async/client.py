import grpc
import asyncio
import argparse

from pb import sample_pb2, sample_pb2_grpc

async def main(port, num_calls):
    async with grpc.aio.insecure_channel(f'localhost:{port}') as channel:
        stub = sample_pb2_grpc.GreeterStub(channel)
        for i in range(num_calls):
            response = await stub.SayHello(sample_pb2.HelloRequest(name=f'World {i+1}'))
            print(f"Response: {response.message}")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Run gRPC client')
    parser.add_argument('--port', type=int, default=12345, help='Port to connect to')
    parser.add_argument('--calls', type=int, default=1, help='Number of times to call SayHello')
    args = parser.parse_args()
    asyncio.run(main(args.port, args.calls))