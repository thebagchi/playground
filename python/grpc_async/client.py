import grpc
import asyncio
import argparse

from pb import sample_pb2, sample_pb2_grpc

async def call_greet(stub, num_calls):
    for i in range(num_calls):
        response = await stub.Greet(sample_pb2.Request(data=f'World {i+1}'))
        print(f"Greet Response: {response.result}")

async def call_client_stream(stub, num_calls):
    async def request_generator():
        for i in range(num_calls):
            yield sample_pb2.Request(data=f'Message {i+1}')
    response = await stub.GreetClientStreaming(request_generator())
    print(f"Client Stream Response: {response.result}")

async def call_server_stream(stub, num_calls):
    response_stream = stub.GreetServerStreaming(sample_pb2.Request(data='Test'))
    async for response in response_stream:
        print(f"Server Stream Response: {response.result}")

async def call_bidi(stub, num_calls):
    async def request_generator():
        for i in range(num_calls):
            yield sample_pb2.Request(data=f'Bidi {i+1}')
            await asyncio.sleep(0.1)  # simulate delay
    response_stream = stub.GreetBidi(request_generator())
    async for response in response_stream:
        print(f"Bidi Response: {response.result}")

async def main(port, rpc, num_calls):
    async with grpc.aio.insecure_channel(f'localhost:{port}') as channel:
        stub = sample_pb2_grpc.GreeterStub(channel)
        if rpc == 'greet':
            await call_greet(stub, num_calls)
        elif rpc == 'client_stream':
            await call_client_stream(stub, num_calls)
        elif rpc == 'server_stream':
            await call_server_stream(stub, num_calls)
        elif rpc == 'bidi':
            await call_bidi(stub, num_calls)
        else:
            print(f"Unknown RPC: {rpc}")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Run gRPC client')
    parser.add_argument('--port', type=int, default=12345, help='Port to connect to')
    parser.add_argument('--rpc', choices=['greet', 'client_stream', 'server_stream', 'bidi'], default='greet', help='RPC to call')
    parser.add_argument('--calls', type=int, default=1, help='Number of calls/messages')
    args = parser.parse_args()
    asyncio.run(main(args.port, args.rpc, args.calls))